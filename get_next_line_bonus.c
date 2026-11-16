/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_bonus.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbaccell <gbaccell@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 12:09:14 by gbaccell          #+#    #+#             */
/*   Updated: 2026/11/16 12:14:38 by gbaccell         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"

static char	*ft_read_stash(int fd, char *stash)
{
	char	*buffer;
	int		bytes;

	buffer = malloc((size_t)BUFFER_SIZE + 1);
	bytes = 1;
	if (buffer == NULL)
		bytes = -1;
	while (bytes > 0 && !ft_strchr(stash, SEP))
	{
		bytes = read(fd, buffer, BUFFER_SIZE);
		if (bytes == -1)
			break ;
		buffer[bytes] = '\0';
		stash = ft_strjoin(stash, buffer);
		if (stash == NULL)
			break ;
	}
	free(buffer);
	if (bytes == -1)
	{
		free(stash);
		return (NULL);
	}
	return (stash);
}

static char	*ft_get_line(char *stash)
{
	int	i;

	if (stash == NULL || stash[0] == '\0')
		return (NULL);
	i = 0;
	while (stash[i] && stash[i] != SEP)
		i++;
	if (stash[i] == SEP)
		i++;
	return (ft_substr(stash, 0, i));
}

static char	*ft_new_stash(char *stash)
{
	int		i;
	char	*new;

	i = 0;
	while (stash[i] && stash[i] != SEP)
		i++;
	if (stash[i] != SEP || stash[i + 1] == '\0')
	{
		free(stash);
		return (NULL);
	}
	new = ft_substr(stash, i + 1, ft_strlen(stash) - i - 1);
	free(stash);
	return (new);
}

char	*get_next_line(int fd)
{
	static char	*stash[MAX_FD];
	char		*line;
	int			has_rest;

	if (fd < 0 || fd >= MAX_FD || BUFFER_SIZE <= 0)
		return (NULL);
	stash[fd] = ft_read_stash(fd, stash[fd]);
	if (stash[fd] == NULL)
		return (NULL);
	line = ft_get_line(stash[fd]);
	if (line == NULL)
	{
		free(stash[fd]);
		stash[fd] = NULL;
		return (NULL);
	}
	has_rest = (stash[fd][ft_strlen(line)] != '\0');
	stash[fd] = ft_new_stash(stash[fd]);
	if (has_rest && stash[fd] == NULL)
	{
		free(line);
		return (NULL);
	}
	return (line);
}
